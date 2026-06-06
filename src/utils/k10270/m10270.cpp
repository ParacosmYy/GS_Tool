#include "k10270/m10270.h"
QVector<double> m10270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
