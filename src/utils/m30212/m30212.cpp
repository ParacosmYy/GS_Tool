#include "m30212/m30212.h"
QVector<double> m30212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
