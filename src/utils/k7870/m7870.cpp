#include "k7870/m7870.h"
QVector<double> m7870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
