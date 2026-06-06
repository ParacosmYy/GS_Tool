#include "m8272/m8272.h"
QVector<double> m8272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
