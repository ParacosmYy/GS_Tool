#include "f8785/m8785.h"
QVector<double> m8785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
