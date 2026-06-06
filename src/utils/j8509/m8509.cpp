#include "j8509/m8509.h"
QVector<double> m8509::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
