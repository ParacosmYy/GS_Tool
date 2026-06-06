#include "s7978/m7978.h"
QVector<double> m7978::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
