#include "i12608/m12608.h"
QVector<double> m12608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
