#include "m24792/m24792.h"
QVector<double> m24792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
