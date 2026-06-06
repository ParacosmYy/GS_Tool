#include "k35390/m35390.h"
QVector<double> m35390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
