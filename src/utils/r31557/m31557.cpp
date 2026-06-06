#include "r31557/m31557.h"
QVector<double> m31557::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
