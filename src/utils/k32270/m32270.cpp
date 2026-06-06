#include "k32270/m32270.h"
QVector<double> m32270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
