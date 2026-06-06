#include "q8716/m8716.h"
QVector<double> m8716::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
