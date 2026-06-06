#include "m14272/m14272.h"
QVector<double> m14272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
