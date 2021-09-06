import learn as oc_learn
import time


class IndependentV1CATeacher(oc_learn.Teacher):
    """
    SubClass of teacher, keeping initialisation and accepts_word as it is,
    but changing partial equivalence and equivalence queries so it is automatic
    """
    
    def __init__(self, ref_behaviour, ref_oc, regex: str or None = None, func: callable or None = None):
        super().__init__.(regex, func)
        self.ref_behaviour = ref_behaviour
        self.ref_oc = ref_oc

    def accepts_word(self, word: str) -> bool:
        return super().accepts_word(word)

    def is_language(self, automata):
        """
        Tell whether the giver V1CA language is the same language as the reference V1CA
        """
        pass

    def is_behaviour(self, automata):
        """
        Tell wheteher the reference behaviour graph is equivalent to 
        """
        pass


def basic_benchmark(alphabet, teacher: IndependentV1CATeacher):
    """
    Launch a basic benchmark on one test and returs the total process time
    """
    t0 = time.time()

    learn_one_counter(alphabet, teacher)

    t1 = time.time()
    dt = t1 - t0
    
    return dt


if __name__ == "__main__":
    """
    Main function
    """

    pass

