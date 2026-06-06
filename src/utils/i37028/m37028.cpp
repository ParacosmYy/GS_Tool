#include "i37028/m37028.h"
QVector<double> m37028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
