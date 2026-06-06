#include "a10760/m10760.h"
QVector<double> m10760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
