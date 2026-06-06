#include "f18785/m18785.h"
QVector<double> m18785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
