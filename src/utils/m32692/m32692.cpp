#include "m32692/m32692.h"
QVector<double> m32692::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
