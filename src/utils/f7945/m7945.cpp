#include "f7945/m7945.h"
QVector<double> m7945::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
