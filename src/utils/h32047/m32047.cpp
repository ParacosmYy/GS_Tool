#include "h32047/m32047.h"
QVector<double> m32047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
