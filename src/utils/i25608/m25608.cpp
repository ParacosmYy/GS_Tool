#include "i25608/m25608.h"
QVector<double> m25608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
