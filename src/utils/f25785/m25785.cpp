#include "f25785/m25785.h"
QVector<double> m25785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
