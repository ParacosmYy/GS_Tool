#include "l35291/m35291.h"
QVector<double> m35291::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
