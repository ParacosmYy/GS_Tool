#include "m29132/m29132.h"
QVector<double> m29132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
