#include "a7880/m7880.h"
QVector<double> m7880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
