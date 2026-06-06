#include "i7988/m7988.h"
QVector<double> m7988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
