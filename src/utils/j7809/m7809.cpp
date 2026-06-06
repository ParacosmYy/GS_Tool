#include "j7809/m7809.h"
QVector<double> m7809::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
