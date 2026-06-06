#include "k31650/m31650.h"
QVector<double> m31650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
