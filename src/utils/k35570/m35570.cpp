#include "k35570/m35570.h"
QVector<double> m35570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
