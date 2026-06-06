#include "a10880/m10880.h"
QVector<double> m10880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
