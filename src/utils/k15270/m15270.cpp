#include "k15270/m15270.h"
QVector<double> m15270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
