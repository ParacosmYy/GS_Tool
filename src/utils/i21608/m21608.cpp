#include "i21608/m21608.h"
QVector<double> m21608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
