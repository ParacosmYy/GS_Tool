#include "g15286/m15286.h"
QVector<double> m15286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
