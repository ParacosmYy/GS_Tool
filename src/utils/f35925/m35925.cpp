#include "f35925/m35925.h"
QVector<double> m35925::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
