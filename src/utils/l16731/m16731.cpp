#include "l16731/m16731.h"
QVector<double> m16731::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
