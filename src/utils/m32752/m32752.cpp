#include "m32752/m32752.h"
QVector<double> m32752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
