#include "m35312/m35312.h"
QVector<double> m35312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
