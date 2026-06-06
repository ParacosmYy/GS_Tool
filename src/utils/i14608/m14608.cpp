#include "i14608/m14608.h"
QVector<double> m14608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
