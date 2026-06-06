#include "m28312/m28312.h"
QVector<double> m28312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
