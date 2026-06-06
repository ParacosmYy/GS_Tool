#include "i37608/m37608.h"
QVector<double> m37608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
