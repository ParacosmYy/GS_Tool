#include "g7846/m7846.h"
QVector<double> m7846::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
