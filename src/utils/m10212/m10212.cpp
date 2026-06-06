#include "m10212/m10212.h"
QVector<double> m10212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
