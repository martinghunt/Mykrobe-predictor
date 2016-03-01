from __future__ import print_function
import sys
from collections import Counter
from mongoengine import connect
from mongoengine.connection import ConnectionError
from pymongo.errors import ServerSelectionTimeoutError
import logging
from atlas.schema import Variant
from atlas.utils import split_var_name
from atlas.utils import flatten
from atlas.utils import unique
from atlas.panelgeneration import AlleleGenerator
from atlas.schema import VariantSet

logger = logging.getLogger(__name__)
logger.setLevel(level=logging.DEBUG)


def get_context(pos, kmer):
    context = []
    for variant in Variant.objects(
            start__ne=pos,
            start__gt=pos - kmer,
            start__lt=pos + kmer):
        for split_variant in variant.split():
            context.append(split_variant)
    return context


def seen_together(variants):
    # Takes a list of variants.
    # Returns a list of variants that appear together (in the same variant set)
    variant_to_samples = {}
    for variant in variants:
        variant_to_samples[variant] = variant.seen_in_samples()

    samples_counter = Counter(flatten(variant_to_samples.values()))
    samples_seen_more_than_once = [
        k for k,
        v in samples_counter.iteritems() if v > 1]
    contexts = []
    for sample in samples_seen_more_than_once:
        vars_together = []
        for variant, samples in variant_to_samples.items():
            if sample in samples:
                vars_together.append(variant)
        if vars_together not in contexts:
            contexts.append(vars_together)
            variants = [var for var in variants if var not in vars_together]
    for var in variants:
        contexts.append([var])
    return contexts + [[]]


def make_variant_probe(al, variant, kmer, DB=None):
    if DB is not None:
        try:
            context = get_context(variant.start, kmer)
        except (ServerSelectionTimeoutError, ConnectionError):
            DB = None
            context = []
            logger.warning(
                "Could not connect to database. Continuing without using genetic backgrounds")
    else:
        context = []
    variant_probe = None
    contexts_seen_together = seen_together(context)
    alts = []
    for context in contexts_seen_together:
        panel = al.create(variant, context)
        ref = panel.ref
        panel.alts
        if variant_probe is not None:
            variant_probe.alts.extend(panel.alts)
        else:
            variant_probe = panel
    variant_probe.alts = unique(variant_probe.alts)
    return variant_probe
