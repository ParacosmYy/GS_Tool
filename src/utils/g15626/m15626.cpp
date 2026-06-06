#include "g15626/m15626.h"
QVector<double> m15626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
