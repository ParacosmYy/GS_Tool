#include "i31588/m31588.h"
QVector<double> m31588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
