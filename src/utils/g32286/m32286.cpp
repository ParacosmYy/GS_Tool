#include "g32286/m32286.h"
QVector<double> m32286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
