#include "k35230/m35230.h"
QVector<double> m35230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
