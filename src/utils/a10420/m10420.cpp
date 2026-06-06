#include "a10420/m10420.h"
QVector<double> m10420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
