#include "p28595/m28595.h"
QVector<double> m28595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
