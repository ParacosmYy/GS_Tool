#include "j18509/m18509.h"
QVector<double> m18509::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
