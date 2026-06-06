#include "j25569/m25569.h"
QVector<double> m25569::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
