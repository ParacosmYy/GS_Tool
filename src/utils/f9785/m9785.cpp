#include "f9785/m9785.h"
QVector<double> m9785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
