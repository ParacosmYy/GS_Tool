#include "g29306/m29306.h"
QVector<double> m29306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
