#include "p9715/m9715.h"
QVector<double> m9715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
