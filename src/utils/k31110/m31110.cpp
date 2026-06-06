#include "k31110/m31110.h"
QVector<double> m31110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
