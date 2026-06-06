#include "s9178/m9178.h"
QVector<double> m9178::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
