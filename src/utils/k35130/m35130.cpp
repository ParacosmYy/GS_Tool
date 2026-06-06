#include "k35130/m35130.h"
QVector<double> m35130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
