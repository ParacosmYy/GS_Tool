#include "g25286/m25286.h"
QVector<double> m25286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
