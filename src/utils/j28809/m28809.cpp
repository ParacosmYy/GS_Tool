#include "j28809/m28809.h"
QVector<double> m28809::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
