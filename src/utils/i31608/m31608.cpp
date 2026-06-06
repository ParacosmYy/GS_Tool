#include "i31608/m31608.h"
QVector<double> m31608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
