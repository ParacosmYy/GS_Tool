#include "s9018/m9018.h"
QVector<double> m9018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
