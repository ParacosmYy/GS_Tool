#include "d31583/m31583.h"
QVector<double> m31583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
